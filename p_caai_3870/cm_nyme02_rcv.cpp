/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:
Version:    1.0
Date:     2019-8-21
Description: 非生产按班能源信息
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
#include "epex.h" 
BM2F_ENTERACE_TELE(cm_nyme02_rcv)

int f_cm_nyme02_rcv(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CModel tcaaia12("TCAAIA12");
	int doFlag = 0;
	CString sqlstr = " ";
	CString action_code = " ";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CDbCommand cmd_inq(conn);
	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tcaaia12.Reset();
			tcaaia12.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			action_code = bcls_rec->Tables[0].Rows[i]["ACTION_CODE"].ToString();
			tcaaia12["EQU_NO"] = bcls_rec->Tables[0].Rows[i]["UNIT_CODE"].ToString();

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

			if (bcls_rec->Tables[0].Rows[i]["TYPE"].ToString() == 'M')
			{
				tcaaia12["BUSI_TYPE"] = "YJ";
			}
			else
			{
				tcaaia12["BUSI_TYPE"] = "TJ";
				if (tcaaia12["EQU_NO"].ToString().Trim() == "ZH")
				{
					tcaaia12["EQU_NO"] = " ";
					tcaaia12["BUSI_TYPE"] = "ZH";
				}
				else
				{
					sqlstr = " select code_line,sub_backlog_code from tcaac14 where instr( CODE, @equ_no ) >0"
						" ORDER BY sub_backlog_code"
						;
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("equ_no", tcaaia12["EQU_NO"].ToString());
					cmd_inq.ExecuteReader();
					while (cmd_inq.Read())
					{
						tcaaia12["DEPT_CODE"] = cmd_inq.GetString(1);
						tcaaia12["SUB_BACKLOG_CODE"] = cmd_inq.GetString(2);
					}
					cmd_inq.Close();
				}

				//平时的能源，如果是电，则插入单价到价格表
				if (tcaaia12["MAT_CODE"].ToString() == "D01" &&tcaaia12["PRICE_UNIT"].ToDecimal() != 0)
				{
					sqlstr = " delete from tcaac12"
						" where 1=1"
						" and DEPT_CODE = @dept_code"
						" and VALID_TIME_START = @prod_date"						
						" and price_terms = 'YJ'"
						" and mat_code = 'D01'"
						;
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("dept_code", tcaaia12["DEPT_CODE"].ToString());
					cmd_inq.Parameters.Set("prod_date", tcaaia12["PROD_DATE"].ToString());
					cmd_inq.ExecuteNonQuery();
					cmd_inq.Close();

					sqlstr = " insert into  tcaac12 ( rec_create_time, dept_code,price_terms, mat_code, mat_name, price_unit,valid_time_start)"
						" values (@datenow, @dept_code,'YJ', 'D01', '电', @price_unit,@prod_date)"						
						;
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("dept_code", tcaaia12["DEPT_CODE"].ToString());
					cmd_inq.Parameters.Set("prod_date", tcaaia12["PROD_DATE"].ToString());
					cmd_inq.Parameters.Set("datenow", datenow);
					cmd_inq.Parameters.Set("price_unit", tcaaia12["PRICE_UNIT"].ToDecimal());
					cmd_inq.ExecuteNonQuery();
					cmd_inq.Close();

				}
			}
			
			if (action_code == "D")
			{
				tcaaia12.TrimOrBlank();
				tcaaia12.Delete("PROD_DATE,SHIFT_GROUP,SHIFT_NO,EQU_NO,MAT_CODE_T,BUSI_TYPE,DEPT_CODE,SUB_BACKLOG_CODE"); //日期，班次，班组，机组号，物料代码
			}
			else
			{
				
				tcaaia12["REC_CREATE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
				tcaaia12["REC_CREATOR"] = s.userid;
				tcaaia12.TrimOrBlank(); //去除多余的空格，如果是null的话字符型赋值空格或是数字小赋0

				tcaaia12.Delete("PROD_DATE,SHIFT_GROUP,SHIFT_NO,EQU_NO,MAT_CODE_T,BUSI_TYPE,DEPT_CODE,SUB_BACKLOG_CODE");
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


