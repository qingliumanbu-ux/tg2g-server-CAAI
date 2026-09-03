/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:
Version:    1.0
Date:     2019-8-21
Description: 生产按炉/计划号能源消耗
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
#include "epex.h" 


BM2F_ENTERACE_TELE(cm_nyme01_rcv)


int f_cm_nyme01_rcv(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CModel tcaaia12("TCAAIA12");
	int doFlag = 0;
	CString sqlstr = " ";
	CString action_code = " ";
	CDbCommand cmd_inq(conn);
	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tcaaia12.Reset();
			tcaaia12.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			action_code = bcls_rec->Tables[0].Rows[i]["ACTION_CODE"].ToString();
			tcaaia12["BUSI_TYPE"] = "SC";
			tcaaia12["START_TIME"] = bcls_rec->Tables[0].Rows[i]["PROD_START_TIME"].ToString();
			tcaaia12["END_TIME"] = bcls_rec->Tables[0].Rows[i]["PROD_END_TIME"].ToString();
			tcaaia12["EQU_NO"] = bcls_rec->Tables[0].Rows[i]["UNIT_CODE"].ToString();
			tcaaia12["PROD_DATE"] = bcls_rec->Tables[0].Rows[i]["PROD_START_TIME"].ToString().SubstringNE(0,8);
			tcaaia12["DEV_POS"] = bcls_rec->Tables[0].Rows[i]["CHARGE_NO"].ToString();

			tcaaia12["MAT_CODE_T"] = bcls_rec->Tables[0].Rows[i]["MAT_CODE"].ToString();
			tcaaia12["MAT_NAME_T"] = bcls_rec->Tables[0].Rows[i]["MAT_NAME"].ToString();

			tcaaia12["MAT_CODE"] = " ";
			//更新sap物料编码
			sqlstr = " select  CODE_DESC_1_CONTENT from tep0002"
				" where CODE = @mat_code_t"
				" AND CODE_class = 'CAN1' "
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("mat_code_t", tcaaia12["MAT_CODE_T"].ToString());
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				tcaaia12["MAT_CODE"] = cmd_inq.GetString(1);
			}
			cmd_inq.Close();
			

			if (tcaaia12["EQU_NO"].ToString().GetLength() == 2)
			{
				tcaaia12["DEPT_CODE"] = "S";
				tcaaia12["SUB_BACKLOG_CODE"] = tcaaia12["EQU_NO"].ToString().Substring(0,1);
			}
			else
			{
				//取对应的产线和工序
				sqlstr = " select CODE_LINE,SUB_BACKLOG_CODE"
					" from tcaac14"
					" where  instr(CODE,@equ_no)>0"
					;
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("equ_no", tcaaia12["EQU_NO"].ToString());
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tcaaia12["DEPT_CODE"] = cmd_inq.GetString(1);
					tcaaia12["SUB_BACKLOG_CODE"] = cmd_inq.GetString(2);
				}
				cmd_inq.Close();
			}

			if (action_code == "D")
			{
				tcaaia12.TrimOrBlank();
				tcaaia12.Delete("PROD_DATE,PLAN_NO,BUSI_TYPE,MAT_CODE_T,EQU_NO,DEV_POS"); //计划号加工序加业务类型
			}
			else
			{
				tcaaia12["REC_CREATE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
				tcaaia12["REC_CREATOR"] = s.userid;
				tcaaia12.TrimOrBlank(); //去除多余的空格，如果是null的话字符型赋值空格或是数字小赋0
				tcaaia12.Delete("PROD_DATE,PLAN_NO,BUSI_TYPE,MAT_CODE_T,EQU_NO,DEV_POS");
				tcaaia12.Insert();
			}
		}
	}
	catch (CDbException& ex)
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}],请联系开发人员", arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;
	}
	catch (CApplicationException& ex)
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strcpy(s.msg, ex.GetMsg());
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}


