/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhoabin
Version:    1.0
Date:     2020-12-09
Description: 针对加工重量进行修改
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caais7_upd2)

int f_caais7_upd2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int   doFlag = 0;
	int   fetchRowCount = 0;


	/* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaais7("TCAAIS7");

	// 数据库SQL操作字符串
	CString  sql("");
	CDbCommand cmd_inq(conn);
	CString sqlstr = "";


	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		//获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();

		for (int i = 0; i < count; i++)
		{
			tcaais7.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			//修改信息
			tcaais7["REC_REVISOR"] = s.userid;   //记录修改责任者
			tcaais7["REC_REVISE_TIME"] = datetime;   //记录修改时刻
			if (tcaais7["SEND_FLAG"].ToString().Trim() != "1")
			{	
				if (tcaais7["REMARK"].ToString().Trim() == "")
				{
					strcpy(s.msg, "请新增备注说明!");
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
				//判断该计划号材料支数全部完成
				sqlstr = " select count(1) from tcaais8"
					" where 1=1"
					" and AFFIRM_FLAG !='1'"
					" and DECIDE_CODE =@decide_code"
					" and plan_no = @plan_no"
					;
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("plan_no", tcaais7["PLAN_NO"].ToString());
				cmd_inq.Parameters.Set("decide_code", tcaais7["DECIDE_CODE"].ToString());
				fetchRowCount = cmd_inq.ExecuteScalar().ToInt32();
				cmd_inq.Close();
				if (fetchRowCount > 0)
				{
					strcpy(s.msg, "还有未确认的材料号信息，不能修改!");
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
				tcaais7["AMT"] = (tcaais7["WT"].ToDecimal() * tcaais7["PRICE_UNIT"].ToDecimal()).Round(2);
				tcaais7["CHEAP_AMT"] = (tcaais7["WT"].ToDecimal() *  tcaais7["CHEAP_RANGE"].ToDecimal()).Round(2);
				

				//修改一条记录
				tcaais7.Update("REC_REVISOR,REC_REVISE_TIME,WT,REMARK,REMARK_1", "OUT_TASK_NO,OUT_UNIT_CODE,RECV_DEPT,PROD_DATE,PLAN_NO,EQU_NO,TYPE_CODE,DECIDE_CODE");
			}
			else
			{
				strcpy(s.msg, "加工量修改失败，发送标记为'1'[已发送]!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

		}
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。" /* _RES("GCRSS0000006")*//*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sql + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		EDLog(1, 1, "[%s]", s.sysmsg);
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}